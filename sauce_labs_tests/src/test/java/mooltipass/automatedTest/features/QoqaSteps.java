package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Qoqa;

public class QoqaSteps {

	Qoqa qoqa = new Qoqa(WebDriverFactory.get());
	
	@When("I login qoqa with '(.*)'")
	public void login(String username){
		qoqa.goToLogin();
		qoqa.enterEmail(username);
		String password=System.getenv().get("QOQAPASS");
		qoqa.enterPassword(password);
		qoqa.submit();
		
	}

	@Then("I should be logged in qoqa")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",qoqa.checkLogin());
	}
	
	@When("I go to qoqa login page")
	public void pressLogin(){
		qoqa.goToLogin();
		Assert.assertTrue("Expected to be at login page", qoqa.checkAtLoginPage());
		
	}
	
	@When("I logout qoqa")
	public void pressLogout(){
		qoqa.logout();
	}
}