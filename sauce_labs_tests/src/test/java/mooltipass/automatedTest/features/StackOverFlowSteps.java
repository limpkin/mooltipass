package mooltipass.automatedTest.features;

import java.util.concurrent.TimeUnit;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;

import cucumber.api.java.en.Given;
import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.StackOverflow;

public class StackOverFlowSteps {
	
	StackOverflow stackOverFlow= new StackOverflow(WebDriverFactory.get());
	
	@Given("I navigate to '(.*)'")
	public void navigateToURL(String url){
		WebDriverFactory.get().get(url);
		stackOverFlow.sleep(1000);
	}
	
	@When("I log in StackOverFlow with '(.*)'")
	public void login(String email){
		stackOverFlow.enterEmail(email);
		String password =System.getenv().get("STACKPASS");
		stackOverFlow.enterPassword(password);
		stackOverFlow.submit();
	}
	@When("I go to StackOverFlow login page")
	public void pressLogin(){
		stackOverFlow.goToLogin();
		Assert.assertTrue("Expected to be at login page", stackOverFlow.checkAtLoginPage());
		
	}
	
	@Then("I should be logged in StackOverFlow")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",stackOverFlow.checkLogin());
	}
	
	@When("I logout StackOverFlow")
	public void pressLogout(){
		stackOverFlow.logout();
	}
}
