package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.PcOstschweiz;

public class PcOstschweizSteps {

	PcOstschweiz pcOstschweiz = new PcOstschweiz(WebDriverFactory.get());
	
	@When("I login pc-ostschweiz with '(.*)'")
	public void login(String username){
		pcOstschweiz.enterEmail(username);
		String password =System.getenv().get("PCOSTPASS");
		pcOstschweiz.enterPassword(password);
		pcOstschweiz.submit();
		
	}

	@Then("I should be logged in pc-ostschweiz")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",pcOstschweiz.checkLogin());
	}
	
	
	@When("I logout pc-ostschweiz")
	public void pressLogout(){
		pcOstschweiz.logout();
	}
}
