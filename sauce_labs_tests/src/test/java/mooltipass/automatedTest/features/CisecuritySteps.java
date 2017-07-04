package mooltipass.automatedTest.features;

import cucumber.api.java.en.Given;
import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Cisecurity;

public class CisecuritySteps {

	Cisecurity cisecurity= new Cisecurity(WebDriverFactory.get());
	
	@When("I login cisecurity with '(.*)'")
	public void login(String email){
		cisecurity.enterEmail(email);
		String password=System.getenv().get("CISPASS");
		cisecurity.enterPassword(password);
		cisecurity.submit();
	}
	@When("I go to cisecurity login page")
	public void pressLogin(){
		Assert.assertTrue("Expected to be at login page", cisecurity.checkAtLoginPage());
		
	}
	
	@Then("I should be logged in cisecurity")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",cisecurity.checkLogin());
	}
	
	@When("I logout cisecurity")
	public void pressLogout(){
		cisecurity.logout();
	}
}
