package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class HardwareFR  extends AbstractPage{
	
	public HardwareFR (WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	@FindBy(xpath = "//a[contains(text(),'identifier')]")
	private WebElement loginBtn;
	@FindBy(xpath = "//input[@name='pseudo']")
	private WebElement email;
	
	@FindBy(xpath = "//input[@name='password']")
	private WebElement password;
	
	@FindBy(xpath = "//input[@name='Valider']")
	private WebElement submitLogin;

	@FindBy(xpath = "//a[contains(text(),'Se déconnecter')]")
	private WebElement logoutBtn;
	
	public void goToLogin(){	
		loginBtn.click();
	}
	
	public void enterEmail(String value){
		email.sendKeys(value);
	}

	public void enterPassword(String value){
		waitUntilAppears(password);
		password.sendKeys(value);
	}
	
	
	public void submit(){

		submitLogin.click();
	}

	public void logout(){		
		clickWithAction(logoutBtn);
	}
	
	public boolean checkLogin(){
		waitUntilAppears(By.xpath("//a[contains(text(),'Se déconnecter')]"));
		return isElementPresent(By.xpath("//a[contains(text(),'Se déconnecter')]"));
	}
	public boolean checkAtLoginPage(){
		return isElementPresent(By.xpath("//input[@name='pseudo']"));
	}

}
